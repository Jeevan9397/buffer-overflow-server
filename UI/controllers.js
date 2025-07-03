angular.module('bfApp')
.controller('LoginCtrl', function($scope, $http, $location) {
  $scope.user = { user: '', pass: '', showPass: false };
  $scope.error = '';

  $scope.togglePass = function() {
    $scope.user.showPass = !$scope.user.showPass;
  };

  $scope.login = function() {
    $http.post('/api/login', {
      user: $scope.user.user,
      pass: $scope.user.pass
    }).then(function(response) {
      if (response.data.status === 'ok') {
        $location.path('/dashboard');
      } else {
        $scope.error = "Login failed. Please try again.";
      }
    }, function() {
      $scope.error = "Invalid username or password.";
    });
  };
})

.controller('DashboardCtrl', function($scope, $http, $location) {
  $scope.records = [];
  $scope.newRecord = {};
  $scope.showAddModal = false;
  $scope.editing = false;
  $scope.selectedRecord = null;
  $scope.selectedRecords = [];
  $scope.lastSelected = null;

  $scope.triggerImport = function () {
  document.getElementById('importFile').click();
};

$scope.import = function () {
  document.getElementById('importFile').click();
};

$scope.importFileChanged = function (input) {
  const file = input.files[0];
  if (!file) return;

  const formData = new FormData();
  formData.append("file", file);

  $http.post("/api/import", formData, {
    transformRequest: angular.identity,
    headers: { "Content-Type": undefined }
  }).then(function (res) {
    alert("Import successful!");
    refreshRecords();
  }, function (err) {
    alert("Import failed.");
    console.error(err);
  });

  // Clear the input so user can re-import same file later
  input.value = '';
};



  // Selection logic: ctrl-click, shift-click, or single
  $scope.selectRecord = function(record, $event) {
    const index = $scope.selectedRecords.indexOf(record);

    if ($event.ctrlKey || $event.metaKey) {
      if (index === -1) {
        $scope.selectedRecords.push(record);
      } else {
        $scope.selectedRecords.splice(index, 1);
      }
    } else if ($event.shiftKey && $scope.lastSelected) {
      const last = $scope.records.indexOf($scope.lastSelected);
      const current = $scope.records.indexOf(record);
      const [start, end] = [Math.min(last, current), Math.max(last, current)];
      $scope.selectedRecords = $scope.records.slice(start, end + 1);
    } else {
      if ($scope.selectedRecords.length === 1 && index !== -1) {
        $scope.selectedRecords = [];
      } else {
        $scope.selectedRecords = [record];
      }
    }

    $scope.lastSelected = record;
    $scope.selectedRecord = $scope.selectedRecords[0] || null;
  };

  function formatDate(isoDate) {
    const d = new Date(isoDate);
    if (!isNaN(d)) {
      const dd = String(d.getDate()).padStart(2, '0');
      const mm = String(d.getMonth() + 1).padStart(2, '0');
      const yyyy = d.getFullYear();
      return `${dd}-${mm}-${yyyy}`;
    }
    return isoDate;
  }

  function parseDate(dobStr) {
  if (!dobStr) return '';

  if (dobStr instanceof Date && !isNaN(dobStr)) {
    const yyyy = dobStr.getFullYear();
    const mm = String(dobStr.getMonth() + 1).padStart(2, '0');
    const dd = String(dobStr.getDate()).padStart(2, '0');
    return `${yyyy}-${mm}-${dd}`;
  }

  // Replace dots with dashes to handle "10.10.1997"
  dobStr = dobStr.replace(/\./g, '-');

  const parts = dobStr.split('-');
  if (parts.length === 3 && parts[2].length === 4) {
    return `${parts[2]}-${parts[1].padStart(2, '0')}-${parts[0].padStart(2, '0')}`;
  }

  return dobStr;
}


  function formatCustomers(customers) {
    return customers.map(c => ({
      id: c.id,
      name: c.name,
      father: c.father,
      nationality: c.nationality,
      dob: c.dob,
      dobFormatted: formatDate(c.dob),
      birthplace: c.birthplace,
      address: c.address,
      mobile: c.mobile,
      email: c.email
    }));
  }

  function refreshRecords() {
    $http.get('/api/customers').then(function(resp) {
      $scope.records = formatCustomers(resp.data.customers);
      $scope.selectedRecords = [];
      $scope.selectedRecord = null;
    });
  }

  refreshRecords();

  $scope.add = function() {
    $scope.newRecord = {};
    $scope.editing = false;
    $scope.showAddModal = true;
  };

  $scope.cancelAdd = function() {
    $scope.showAddModal = false;
    $scope.editing = false;
    $scope.newRecord = {};
  };

  $scope.triggerEdit = function() {
    if ($scope.selectedRecords.length === 0) {
      alert("Please select a record to edit.");
      return;
    }
    $scope.editRecord($scope.selectedRecords[0]);
  };

  $scope.editRecord = function(record) {
    $scope.newRecord = angular.copy(record);
    $scope.newRecord.dob = new Date(record.dob);
    $scope.editing = true;
    $scope.showAddModal = true;
  };

  $scope.saveRecord = function() {
    const payload = {
      name: $scope.newRecord.name,
      father: $scope.newRecord.father,
      nationality: $scope.newRecord.nationality,
      dob: parseDate($scope.newRecord.dob),
      birthplace: $scope.newRecord.birthplace,
      address: $scope.newRecord.address,
      mobile: $scope.newRecord.mobile,
      email: $scope.newRecord.email
    };

    const endpoint = $scope.editing
      ? `/api/customers/${$scope.newRecord.id}`
      : '/api/customers';

    const method = $scope.editing ? $http.put : $http.post;

    method(endpoint, payload).then(function() {
      refreshRecords();
      $scope.showAddModal = false;
      $scope.editing = false;
      $scope.newRecord = {};
    }, function() {
      alert("Error saving record.");
    });
  };

  $scope.export = function () {
    let ids = [];

    if ($scope.selectedRecords && $scope.selectedRecords.length > 0) {
      ids = $scope.selectedRecords.map(r => r.id);
    }

    let url = '/api/export';
    if (ids.length > 0 && ids.length !== $scope.records.length) {
      url += '?ids=' + ids.join(',');
    }

    const a = document.createElement('a');
    a.href = url;
    a.download = 'customers.csv';
    a.click();
  };

  $scope.delete = function() {
    if ($scope.selectedRecords.length === 0) {
      alert("Please select at least one record to delete.");
      return;
    }

    if (!confirm(`Delete ${$scope.selectedRecords.length} selected record(s)?`)) return;

    const toDelete = [...$scope.selectedRecords];

    let deletedCount = 0;
    toDelete.forEach((rec, idx) => {
      $http.delete(`/api/customers/${rec.id}`).then(() => {
        deletedCount++;
        if (deletedCount === toDelete.length) {
          refreshRecords();
        }
      }, () => {
        alert("Error deleting record with ID: " + rec.id);
      });
    });
  };

  $scope.logout = function() {
    $location.path('/login');
  };
});
